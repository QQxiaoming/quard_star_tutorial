(: Name: Comp-notation-7 :)
(: Description: Evaluation of Notation Comparison operator (eq) and used expression as argument to fn:not function, returns false. :)

(: insert-start :)
import schema namespace myns="http://www.example.com/notation";
declare variable $input-context external;
(: insert-end :)

fn:not(exactly-one($input-context//*:NOTATION1[1]) eq exactly-one($input-context//*:NOTATION3[1]))
