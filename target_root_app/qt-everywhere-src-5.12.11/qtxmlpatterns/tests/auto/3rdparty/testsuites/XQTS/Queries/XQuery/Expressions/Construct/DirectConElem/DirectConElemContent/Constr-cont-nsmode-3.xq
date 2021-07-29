(: Name: Constr-cont-nsmode-3 :)
(: Written by: Andreas Behm :)
(: Description: Copy-namespace mode preserve, no-inherit :)

declare copy-namespaces preserve, no-inherit;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<y xmlns:inherit="http://www.example.com/inherit">{$input-context}</y>/x/z
