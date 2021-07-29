(: name : user-defined-5 :)
(: description : Evaluation of constructor function operation on simple user defined data type :)
(: restricted from xs:string.:)

(: insert-start :)
import schema namespace myType="http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context1 external;
(: insert-end :)


fn:concat(myType:stringBased("valid value 4"),myType:stringBased("valid value 1"))
