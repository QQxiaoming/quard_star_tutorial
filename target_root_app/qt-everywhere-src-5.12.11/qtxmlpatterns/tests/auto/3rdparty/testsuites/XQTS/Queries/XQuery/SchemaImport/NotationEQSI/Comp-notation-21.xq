(: Name: Comp-notation-21 :)
(: Description: Evaluation of expression of Notation Comparison operator (eq) used as argument to "fn:string" function. :)

(: insert-start :)
import schema namespace myns="http://www.example.com/notation";
declare variable $input-context external;
(: insert-end :)

fn:string(exactly-one($input-context//*:NOTATION1[1]) eq exactly-one($input-context//*:NOTATION2[1]))
