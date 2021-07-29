(:*******************************************************:)
(: Description: Test XQST0093 by importing a module with a circular dependency :)
(:********************************************************** :)

(: insert-start :)
import module namespace errata8_2a="http://www.w3.org/TestModules/errata8_2a";
declare variable $input-context external;
(: insert-end :)

errata8_2a:fun()
