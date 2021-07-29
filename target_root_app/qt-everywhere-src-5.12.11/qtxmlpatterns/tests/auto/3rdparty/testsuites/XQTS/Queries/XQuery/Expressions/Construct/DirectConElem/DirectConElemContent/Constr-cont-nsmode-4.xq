(: Name: Constr-cont-nsmode-4 :)
(: Written by: Andreas Behm :)
(: Description: Copy-namespace mode no-preserve, no-inherit :)

declare copy-namespaces no-preserve, no-inherit;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<y xmlns:inherit="http://www.example.com/inherit">{$input-context}</y>/x/z
