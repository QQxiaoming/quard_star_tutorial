(:*******************************************************:)
(: Description: Test XQST0036 for unknown attribute declaration in imported function parameter type :)
(:********************************************************** :)

(: insert-start :)
import module namespace errata6_6="http://www.w3.org/TestModules/errata6-6";
declare variable $input-context external;
(: insert-end :)

errata6_6:fun(attribute attr {})
