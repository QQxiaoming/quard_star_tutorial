(: name : user-defined-1 :)
(: description : Evaluation of simple user defined data type restricted from xs:integer.:)

(: insert-start :)
import schema namespace myType="http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context1 external;
(: insert-end :)


let $value := myType:sizeType(1)
return $value