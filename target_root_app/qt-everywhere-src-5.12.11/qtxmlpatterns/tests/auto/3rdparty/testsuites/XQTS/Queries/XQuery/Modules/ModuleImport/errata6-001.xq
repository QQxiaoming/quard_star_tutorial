(:*******************************************************:)
(: Description: Test XQST0036 for unknown simple type in imported function parameter type :)
(:********************************************************** :)

(: insert-start :)
import module namespace errata6_1="http://www.w3.org/TestModules/errata6-1";
declare variable $input-context external;
(: insert-end :)

errata6_1:hatsize-add(3, 5)
