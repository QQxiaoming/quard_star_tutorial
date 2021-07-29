(:*******************************************************:)
(: Description: Test XQST0036 for unknown element declaration in imported function parameter type :)
(:********************************************************** :)

(: insert-start :)
import module namespace errata6_5="http://www.w3.org/TestModules/errata6-5";
declare variable $input-context external;
(: insert-end :)

errata6_5:fun(<E6-Root/>)
