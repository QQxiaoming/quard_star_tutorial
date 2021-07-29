(: name : user-defined-3 :)
(: description : Evaluation of constructor function used as part of addition operation user defined data type restricted :)
(: from xs:integer.:)

(: insert-start :)
import schema namespace myType="http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context1 external;
(: insert-end :)


let $value := myType:sizeType(1) + myType:sizeType(2)
return $value