(: Name: Constr-cont-nsmode-2 :)
(: Written by: Andreas Behm :)
(: Description: Copy-namespace mode no-preserve, inherit :)

declare copy-namespaces no-preserve, inherit;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<y xmlns:inherit="http://www.example.com/inherit">{$input-context}</y>/x/z
