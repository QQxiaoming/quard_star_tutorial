(:*******************************************************:)
(: Description: Test of XQST0036 for unknown simple type in imported global variable :)
(:********************************************************** :)

(: insert-start :)
import module namespace errata6_4="http://www.w3.org/TestModules/errata6-4";
declare variable $input-context external;
(: insert-end :)

$errata6_4:var
