(:*******************************************************:)
(: Test: modules-2.xq                                    :)
(: Description: Same Namespace used in imported and importing modules. :)
(:********************************************************** :)

(: insert-start :)
import module namespace defs="http://www.w3.org/TestModules/defs";
declare namespace foo = "http://example.org";
declare variable $input-context external;
(: insert-end :)

<foo:anElement>some Content</foo:anElement>