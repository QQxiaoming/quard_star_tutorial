(: name : user-defined-10 :)
(: description : Use constructor function to use addition operation on two user defined types. :)
(: Restricted from xs:integer and xs:float:)

(: insert-start :)
import schema namespace myType="http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context1 external;
(: insert-end :)


myType:sizeType(16) + myType:floatBased (16)