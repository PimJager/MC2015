{-# LANGUAGE OverloadedStrings #-}
module Main where

import Control.Monad.Writer
import qualified Data.Text as T
import Data.Maybe

type ScreenFile = [[Char]] -- [String]

data Content = Empty | Block | Man | Goal | BlockOnGoal | ManOnGoal | Wall
    deriving (Show,Eq)
type Position = (Content, Int, Int)
type Screen = [Position]

type SMVWriter = Writer T.Text ()

--blocks are named after their starting positions
blockX :: Int -> Int -> T.Text
blockX x y = "bl_" <<< showT x <<< "_" <<< showT y <<< "_X"
blockY :: Int -> Int -> T.Text
blockY x y = "bl_" <<< showT x <<< "_" <<< showT y <<< "_Y"
range :: Int -> Int -> T.Text
range min max = showT min <<< ".." <<< showT max

posC :: (Content -> Bool) -> Screen -> [(Int,Int)]
posC f = map (\(_,x,y) -> (x,y)) . filter (\(t,_,_) -> f t)

main :: IO ()
main = do 
    file <- readFile "../screens/screen.2000"
    let scr = decode $ lines file
    putStr $ T.unpack $ execWriter $ env scr
        where
            env scr = do
                module_
                vars scr
                define
                Main.init scr
                inBounds scr
                noWalls scr
                noOverlap scr

getWidth :: Screen -> Int
getWidth = foldr (\(_,w,_) m -> max w m) 0

getHeight :: Screen -> Int
getHeight = foldr (\(_,_,h) m -> max h m) 0

decode :: ScreenFile -> Screen
decode f = let withPos = zip [0..] (map (zip [0..]) f) in concat $ map decodeLine withPos
    where
        decodeLine :: (Int, [(Int, Char)]) -> [Position]
        decodeLine (y, line) = map (decodeChar y) line
        decodeChar :: Int -> (Int, Char) -> Position
        decodeChar y (x, c) = (d c, x, y)
        d :: Char -> Content
        d ' ' = Empty
        d '@' = Man
        d '.' = Goal
        d '#' = Wall
        d '$' = Block
        d '*' = BlockOnGoal
        d '+' = ManOnGoal
        d c   = error $ "Undefined char: " ++ [c]

-- 
module_ :: SMVWriter
module_ = tellLn "MODULE main"

vars :: Screen -> SMVWriter
vars scr = do
    tellLn "VAR"
    tellLn1 "dir: {up, left, down, right};"
    mapM_ tellVar scr
    tellLn1 $ "mX: " <<< range 0 w <<< ";"
    tellLn1 $ "mY: " <<< range 0 h <<< ";"
    where
        w = getWidth scr
        h = getHeight scr
        tellVar :: Position -> SMVWriter
        tellVar (Block, x, y)   = tellLn1 (blockX x y <<< ": " <<< range 0 w <<< ";") 
                                    >> tellLn1 (blockY x y <<< ": " <<< range 0 h <<< ";")
        tellVar (BlockOnGoal, x ,y) = tellVar (Block, x, y) --doesn't matter that its on a goal here
        tellVar _               = return ()

define :: SMVWriter
define = do
    tellLn "DEFINE"
    tellLn1 "dx := case"
    mapM_ tellLn2 ["dir = left: -1;", "dir = right: 1;", "TRUE: 0;", "esac;"]
    tellLn1 "dy := case"
    mapM_ tellLn2 ["dir = up: -1;", "dir = down: 1;", "TRUE: 0;", "esac;"]

init :: Screen -> SMVWriter
init src = do
    tellLn "INIT"
    tellLn $ (T.intercalate " & " $ mapMaybe init' src) <<< ";"
    where
        init' :: Position -> Maybe T.Text
        init' (Block, x, y) = Just $ blockX x y <<< "=" <<< showT x <<< " & " 
                                <<< blockY x y<<< "=" <<< showT y 
        init' (BlockOnGoal, x, y) = init' (Block, x, y)
        init' (Man, x, y)   = Just $ "mX=" <<< showT x <<< " & mY=" <<< showT y
        init' _             = Nothing

-- All blocks and the man should stay within the board
-- the blocks can not overlap
-- the man and the blocks can not overlap with the walls
inBounds :: Screen -> SMVWriter
inBounds scr = do
    tellLn "INVAR"
    tellLn1 "--the blocks and the man should stay within bounds"
    tellLn1 $ T.intercalate "\n\t& " $ 
                map tellInside (posC (\t -> t == Block || t == BlockOnGoal) scr)
    tellLn1 $ "& "<<<inRange "mX" 0 w <<< " & " <<< inRange "mY" 0 h <<< ";"
    where
        w = getWidth scr
        h = getHeight scr
        tellInside :: (Int,Int) -> T.Text
        tellInside (x, y) = "("<<<inRange (blockX x y) 0 w <<< " & "
                                                <<<inRange (blockY x y) 0 h <<< ")"
        inRange :: T.Text -> Int -> Int -> T.Text
        inRange v min max = "("<<<v<<<">="<<<showT min<<<" & "<<<v<<<"<="<<<showT max<<<")"

noWalls :: Screen -> SMVWriter
noWalls scr = do 
    tellLn "INVAR"
    tellLn1 "--the blocks and the man can not overlap with the walls"
    tellLn1 $ T.intercalate "\n\t& " $ map notPos $ posC (==Wall) scr
    where
        notPos :: (Int, Int) -> T.Text
        notPos (x,y) = (T.intercalate " & " $ 
                            map (notPos' (x,y)) (posC (\t -> t == Block || t == BlockOnGoal) scr))
                        <<< " & !(mX="<<<showT x<<<" & mY="<<<showT y<<<")"
        notPos' :: (Int, Int) -> (Int, Int) -> T.Text
        notPos' (wx, wy) (bx, by) = "!("<<<blockX bx by<<<"="<<<showT wx<<<" & "
                                    <<<blockY bx by<<<"="<<<showT wy<<<")"

noOverlap :: Screen -> SMVWriter
noOverlap src = do
    tellLn "INVAR"
    tellLn1 "--Blocks can not overlap"
    where
        noOverlap' :: (Int, Int) -> (Int, Int) -> T.Text
        noOverlap' (ax, ay) (bx, by) = "!("<<<blockX ax ay<<<"="<<<blockX bx by<<<" & "
                                            <<<blockY ax ay<<<"="<<<blockY bx by<<<")"
        

-- Helper functions for using the SMVWriter

tellLn :: T.Text -> Writer T.Text ()
tellLn w = tell (w `T.snoc` '\n')
tellLn1 w = tell "\t" >> tellLn w
tellLn2 w = tell "\t\t" >> tellLn w

showT :: (Show a) => a -> T.Text
showT = T.pack . show

(<<<) = T.append