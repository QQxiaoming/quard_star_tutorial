(:*******************************************************:)
(: Description: Test XQST0093 by importing a module with a circular dependency :)
(:********************************************************** :)

(: insert-start :)
import module namespace errata8_1a="http://www.w3.org/TestModules/errata8_1a";
declare variable $input-context external;
(: insert-end :)

errata8_1a:fun()
