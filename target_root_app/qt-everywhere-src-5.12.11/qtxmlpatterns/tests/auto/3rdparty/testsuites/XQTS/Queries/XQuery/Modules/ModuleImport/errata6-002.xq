(:*******************************************************:)
(: Description: Test XQST0036 for unknown simple type in imported function return type :)
(:********************************************************** :)

(: insert-start :)
import module namespace errata6_2="http://www.w3.org/TestModules/errata6-2";
declare variable $input-context external;
(: insert-end :)

errata6_2:hatsize-add(3, 5)
