(: name : user-defined-2 :)
(: description : Evaluation of constructor function vioalation on user defined data type restricted from xs:integer.:)

(: insert-start :)
import schema namespace myType="http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context1 external;
(: insert-end :)


let $value := myType:sizeType(20)
return $value