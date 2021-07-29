(: name : user-defined-4 :)
(: description : Evaluation of simple user defined data type restricted from xs:string.:)

(: insert-start :)
import schema namespace myType="http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context1 external;
(: insert-end :)


let $value := myType:stringBased("valid value 4")
return $value