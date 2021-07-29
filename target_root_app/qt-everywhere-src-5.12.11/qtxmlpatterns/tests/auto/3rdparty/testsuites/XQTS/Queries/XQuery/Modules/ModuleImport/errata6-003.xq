(:*******************************************************:)
(: Description: Test lack of XQST0036 for known simple type in imported function return type :)
(:********************************************************** :)

(: insert-start :)
import module namespace errata6_2="http://www.w3.org/TestModules/errata6-2";
import schema namespace a = "http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context external;
(: insert-end :)

errata6_2:hatsize-add(3, 5)
