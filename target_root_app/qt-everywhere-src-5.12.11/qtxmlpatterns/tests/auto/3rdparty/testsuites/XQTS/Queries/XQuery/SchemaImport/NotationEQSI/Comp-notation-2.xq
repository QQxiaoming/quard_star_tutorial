(: Name: Comp-notation-2 :)
(: Written by: Andreas Behm :)
(: Description: notation comparison :)

(: insert-start :)
import schema namespace myns="http://www.example.com/notation";
declare variable $input-context external;
(: insert-end :)

exactly-one($input-context//*:NOTATION1[1]) eq exactly-one($input-context//*:NOTATION2[1])
