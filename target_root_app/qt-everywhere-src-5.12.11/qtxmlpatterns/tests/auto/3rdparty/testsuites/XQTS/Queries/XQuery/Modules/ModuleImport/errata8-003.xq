(:*******************************************************:)
(: Description: Test lack of XQST0093 by importing a module without a circular dependency :)
(:********************************************************** :)

(: insert-start :)
import module namespace errata8_3a="http://www.w3.org/TestModules/errata8_3a";
declare variable $input-context external;
(: insert-end :)

errata8_3a:fun()
