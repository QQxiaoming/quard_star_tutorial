(: Name: Comp-notation-5 :)
(: Description: notation comparison using "ne" :)

(: insert-start :)
import schema namespace myns="http://www.example.com/notation";
declare variable $input-context external;
(: insert-end :)

exactly-one($input-context//*:NOTATION1[1]) ne exactly-one($input-context//*:NOTATION2[1])
