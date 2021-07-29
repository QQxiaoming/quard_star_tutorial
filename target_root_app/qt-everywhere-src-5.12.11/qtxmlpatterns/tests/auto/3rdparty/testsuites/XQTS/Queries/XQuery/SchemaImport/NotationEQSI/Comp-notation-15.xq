(: Name: Comp-notation-15 :)
(: Description: Evaluation of Notation Comparison operator (eq) and used in boolean expression with "fn:true" and "or" . :)

(: insert-start :)
import schema namespace myns="http://www.example.com/notation";
declare variable $input-context external;
(: insert-end :)

(exactly-one($input-context//*:NOTATION1[1]) eq exactly-one($input-context//*:NOTATION2[1])) or fn:true()
