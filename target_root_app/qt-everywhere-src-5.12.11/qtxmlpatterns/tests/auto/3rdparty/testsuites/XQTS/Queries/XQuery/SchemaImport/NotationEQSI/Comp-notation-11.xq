(: Name: Comp-notation-11 :)
(: Description: Evaluation of Notation Comparison operator (eq) and used expression as argument to fn:boolean function. :)

(: insert-start :)
import schema namespace myns="http://www.example.com/notation";
declare variable $input-context external;
(: insert-end :)

fn:boolean(exactly-one($input-context//*:NOTATION1[1]) eq exactly-one($input-context//*:NOTATION2[1]))
