(: Name: Comp-notation-18 :)
(: Description: Evaluation of Notation Comparison operator (ne) and used in boolean expression with "fn:false" and "and" . :)

(: insert-start :)
import schema namespace myns="http://www.example.com/notation";
declare variable $input-context external;
(: insert-end :)

(exactly-one($input-context//*:NOTATION1[1]) ne exactly-one($input-context//*:NOTATION2[1])) and fn:false()
