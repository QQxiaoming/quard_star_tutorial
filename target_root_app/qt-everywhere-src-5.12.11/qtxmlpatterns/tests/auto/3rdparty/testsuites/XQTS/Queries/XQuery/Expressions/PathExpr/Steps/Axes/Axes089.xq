xquery version "1.0";
declare namespace saxon="http://example.com/VendorNamespace";
declare namespace tour="http://example.com/Knight'sTour";

(:
    XQuery program to perform a knight's tour of the chessboard.
    Author: Michael H. Kay
    Date: 26 June 2003
    
    This version modified to use XQuery 1.0, with sequences and functions.

    This query does not use a source document.
    There is an optional parameter, start, which can be set to any square on the
    chessboard, e.g. a3 or h5. XQuery does not allow parameters to be given a
    default value, so the parameter is mandatory.
    
    There is a second optional parameter, end, which indicates that the processing should stop
    after a given number of steps. This can be used to animate the display of the tour. This
    works especially well when the query is compiled into a Java servlet.

    The output is an HTML display of the completed tour.

    Internally, the following data representations are used:
    * A square on the chessboard: represented as a number in the range 0 to 63
    * A state of the chessboard: a sequence of 64 integers, each containing a move number. 
      A square that has not been visited yet is represented by a zero.
    * A set of possible moves: represented as a sequence of integers,
    * each integer representing the number of the destination square
      
:)

declare option saxon:default "'a1'";
declare variable $start as xs:string := 'a1';

declare option saxon:default "'64'";
declare variable $end as xs:string := '64';
declare variable $endd as xs:integer := xs:integer($end); 

(: start-column is an integer in the range 0-7 :)

declare variable $start-column as xs:integer :=
    xs:integer(translate(substring($start, 1, 1),
            'abcdefgh', '01234567'));

(: start-row is an integer in the range 0-7, with zero at the top :)

declare variable $start-row as xs:integer :=
    8 - xs:integer(substring($start, 2, 1));
    
declare function tour:main () as element() {

    (: This function controls the processing. It does not access the source document. :)

    (: Validate the input parameter :)

    if (not(string-length($start)=2) or
        not(translate(substring($start,1,1), 'abcdefgh', 'aaaaaaaa')='a') or
        not(translate(substring($start,2,1), '12345678', '11111111')='1'))
    then
        error((), "Invalid start parameter: try say 'a1' or 'g6'")
    else
    
    if (not($endd = 1 to 64)) 
    then
        error((), "Invalid end parameter: must be in range 1 to 64")
    else

    (: Set up the empty board :)

    let $empty-board as xs:integer* := 
        for $i in (1 to 64) return 0
    
    (: Place the knight on the board at the chosen starting position :)
    
    let $initial-board as xs:integer* :=
        tour:place-knight(1, $empty-board, $start-row * 8 + $start-column)
    
    (: Evaluate the knight's tour :)

    let $final-board as xs:integer* :=
       tour:make-moves(2, $initial-board, $start-row * 8 + $start-column)

    (: produce the HTML output :)
    
    return tour:print-board($final-board)
};

declare function tour:place-knight (
                    $move as xs:integer,
                    $board as xs:integer*,
                    $square as xs:integer (: range 0 to 63 :)
                  ) as xs:integer* {

    (: This function places a knight on the board at a given square. The returned value is
         the supplied board, modified to indicate that the knight reached a given square at a given
         move :)

    for $i in 1 to 64 return
        if ($i = $square + 1) then $move else $board[$i]

};

declare function tour:make-moves (
                    $move as xs:integer,
                    $board as xs:integer*,
                    $square as xs:integer (: range 0 to 63 :)
                ) as xs:integer* {

    (: This function takes the board in a given state, decides on the next move to make,
         and then calls itself recursively to make further moves, until the knight has completed
         his tour of the board. It returns the board in its final state. :)

    (: determine the possible moves that the knight can make :)

    let $possible-move-list as xs:integer* := 
        tour:list-possible-moves($board, $square)

    (: try these moves in turn until one is found that works :)

    return tour:try-possible-moves($move, $board, $square, $possible-move-list)
};

declare function tour:try-possible-moves (
                    $move as xs:integer,
                    $board as xs:integer*,
                    $square as xs:integer, (: range 0 to 63 :)
                    $possible-moves as xs:integer* )
                as xs:integer* {

    (:   This function tries a set of possible moves that the knight can make
         from a given position. It determines the best move as the one to the square with
         fewest exits. If this is unsuccessful then it can backtrack and
         try another move; however this turns out rarely to be necessary. 
         
         The function makes the selected move, and then calls make-moves() to make
         subsequent moves, returning the final state of the board. :)

         if (count($possible-moves)!=0)
                then tour:make-best-move($move, $board, $square, one-or-more($possible-moves))
                else ()

         (: if there is no possible move, we return the special value () as the final state
             of the board, to indicate to the caller that we got stuck :)
};

declare function tour:make-best-move (
                    $move as xs:integer,
                    $board as xs:integer*,
                    $square as xs:integer, (: range 0 to 63 :)
                    $possible-moves as xs:integer+ )
                as xs:integer* {
                
    (: this function, given the state of the board and a set of possible moves,
       determines which of the moves is the best one. It then makes this move,
       and proceeds recursively to make further moves, eventually returning the
       final state of the board. :)            
        
    (:  if at least one move is possible, find the best one :)

    let $best-move as xs:integer :=
        tour:find-best-move($board, $possible-moves, 9, 999)

    (: find the list of possible moves excluding the best one :)

    let $other-possible-moves as xs:integer* :=
        $possible-moves[. != $best-move]

    (: update the board to make the move chosen as the best one :)

    let $next-board as xs:integer* :=
        tour:place-knight($move, $board, $best-move)
    
    (: now make further moves using a recursive call, until the board is complete :)

    let $final-board as xs:integer* :=
        if ($move < $endd) (:count($next-board[.=0])!=0:) 
                    then tour:make-moves($move+1, $next-board, $best-move)
                    else $next-board

    (:   if the final board has the special value '()', we got stuck, and have to choose
         the next best of the possible moves. This is done by a recursive call. I thought
         that the knight never did get stuck, but it does: if the starting square is f1,
         the wrong choice is made at move 58, and needs to be reversed. :)

    return
        if (empty($final-board))
        then tour:try-possible-moves($move, $board, $square, $other-possible-moves)
        else $final-board
        
};

declare function tour:find-best-move (
                    $board as xs:integer*,
                    $possible-moves as xs:integer+,
                    $fewest-exits as xs:integer,
                    $best-so-far as xs:integer )
                as xs:integer {

    (:  This function finds from among the possible moves, the one with fewest exits.
         It calls itself recursively. :)
         
    (:  split the list of possible moves into the first move and the rest of the moves :)

    let $trial-move as xs:integer := 
        $possible-moves[1]
    
    let $other-possible-moves as xs:integer* :=
        $possible-moves[position() > 1]

    (: try making the first move :)

    let $trial-board as xs:integer* :=
        tour:place-knight(99, $board, $trial-move)

    (: see how many moves would be possible the next time :)

    let $trial-move-exit-list as xs:integer* :=
        tour:list-possible-moves($trial-board, $trial-move)

    let $number-of-exits as xs:integer :=
        count($trial-move-exit-list)

    (:  determine whether this trial move has fewer exits than those considered up till now :)

    let $minimum-exits as xs:integer :=
        min(($number-of-exits, $fewest-exits))

    (:  determine which is the best move (the one with fewest exits) so far :)

    let $new-best-so-far as xs:integer :=
        if ($number-of-exits < $fewest-exits)
            then $trial-move
            else $best-so-far  

    (:  if there are other possible moves, consider them too, using a recursive call.
        Otherwise return the best move found. :)

    return
        if (count($other-possible-moves)!=0)
            then tour:find-best-move($board, one-or-more($other-possible-moves), 
                                            $minimum-exits, $new-best-so-far)
            else $new-best-so-far

};

declare function tour:list-possible-moves (
                $board as xs:integer*,
                $square as xs:integer )
            as xs:integer* {

    (:   This function, given the knight's position on the board, returns the set of squares
         he can move to. The squares will be ones that have not been visited before :)
            
    let $row as xs:integer := $square idiv 8
    let $column as xs:integer := $square mod 8

    return
        (if ($row > 1 and $column > 0 and $board[($square - 17) + 1]=0)
            then $square - 17 else (),
         if ($row > 1 and $column < 7 and $board[($square - 15) + 1]=0)
            then $square - 15 else (),
         if ($row > 0 and $column > 1 and $board[($square - 10) + 1]=0)
            then $square - 10 else (),
         if ($row > 0 and $column < 6 and $board[($square - 6) + 1]=0)
            then $square - 6 else (),
         if ($row < 6 and $column > 0 and $board[($square + 15) + 1]=0)
            then $square + 15 else (),
         if ($row < 6 and $column < 7 and $board[($square + 17) + 1]=0)
            then $square + 17 else (),
         if ($row < 7 and $column > 1 and $board[($square + 6) + 1]=0)
            then $square + 6 else (),
         if ($row < 7 and $column < 6 and $board[($square + 10) + 1]=0)
            then $square + 10 else () )

};

declare function tour:print-board (
                $board as xs:integer* )
            as element()
{
    (: Output the board in HTML format :)

    <html>
    <head>
        <title>Knight's tour</title>
    </head>
    <body>
    <div align="center">
    <h1>Knight's tour starting at {$start}</h1>
    <table border="1" cellpadding="4">
        {for $row in 0 to 7 return
           <tr>
              {for $column in 0 to 7
                let $color :=
                          if ((($row + $column) mod 2)=1)
                          then 'xffff44' 
                          else 'white' return
                <td align="center" bgcolor="{$color}" width="22">{
                  let $n := $board[$row * 8 + $column + 1]
                  return 
                      if ($endd != 64 and $n = $endd)
                      then <b>{$n}</b>
                      else if ($n = 0)
                      then "&#xa0;"
                      else $n
                }</td>
              }
           </tr>
        }
    </table>
    <p>{
        if ($endd != 64) 
        then
          <a href="Tour?start={$start}&amp;end={$endd+1}">Step</a>
        else ()
    }</p>    
    </div>
    </body>
    </html>
};

tour:main()

