(: name : user-defined-6 :)
(: description : use constructor function for user defined type and cast value to base type (xs:integer) :)
(: restricted from xs:integer.:)

(: insert-start :)
import schema namespace myType="http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context1 external;
(: insert-end :)


myType:sizeType(2) cast as xs:integer