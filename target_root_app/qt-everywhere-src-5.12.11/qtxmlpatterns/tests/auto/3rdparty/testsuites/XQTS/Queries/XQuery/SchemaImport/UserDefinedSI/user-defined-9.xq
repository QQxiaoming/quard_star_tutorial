(: name : user-defined-9 :)
(: description : Use constructor function to evaluate if two user defined types are castable to another :)
(: restricted from xs:integer and xs:float:)

(: insert-start :)
import schema namespace myType="http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context1 external;
(: insert-end :)


myType:sizeType(16) castable as myType:floatBased