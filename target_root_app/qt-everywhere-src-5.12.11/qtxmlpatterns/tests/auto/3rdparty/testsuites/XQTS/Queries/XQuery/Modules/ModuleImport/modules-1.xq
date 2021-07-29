(:*******************************************************:)
(: Test: modules-1.xq                                    :)
(: Description:  Namespaces from imported modules are not visible. :)
(:********************************************************** :)

(: insert-start :)
import module namespace defs="http://www.w3.org/TestModules/defs";
declare variable $input-context external;
(: insert-end :)

<foo:anElement>some Content</foo:anElement>
