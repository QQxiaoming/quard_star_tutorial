(: name : user-defined-8 :)
(: description : Use constructor function to evaluate if two user defined types can be casted to another.:)
(: Restricted from xs:integer and xs;float.:)

(: insert-start :)
import schema namespace myType="http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context1 external;
(: insert-end :)


myType:sizeType(16) cast as myType:floatBased