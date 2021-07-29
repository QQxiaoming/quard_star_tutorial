(: name : value-comparison-4 :)
(: description : Evaluation of numeric value comparison involving type promotion to least common type.:)
(: Uses "ne" operator between integer and float types. :)

(: insert-start :)
import schema namespace myType="http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context1 external;
(: insert-end :)

myType:hatsize(5) ne myType:shoesize(5)