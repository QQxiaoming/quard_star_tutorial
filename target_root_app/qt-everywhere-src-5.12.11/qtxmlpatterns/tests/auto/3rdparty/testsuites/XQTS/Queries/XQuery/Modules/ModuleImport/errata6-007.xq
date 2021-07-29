(:*******************************************************:)
(: Description: Test XQST0036 for unknown complex type in imported function parameter type :)
(:********************************************************** :)

(: insert-start :)
import module namespace errata6_7="http://www.w3.org/TestModules/errata6-7";
declare variable $input-context external;
(: insert-end :)

errata6_7:fun(<E6/>)
