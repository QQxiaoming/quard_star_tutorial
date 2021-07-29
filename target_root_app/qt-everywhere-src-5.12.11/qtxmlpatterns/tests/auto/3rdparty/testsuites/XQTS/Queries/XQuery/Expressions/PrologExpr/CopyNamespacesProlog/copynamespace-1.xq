(: Name: copynamespace-1.xq :)
(: Description: Evaluation of a prolog with more than one copy-namespace declaration.:)

declare copy-namespaces preserve, no-inherit;
declare copy-namespaces no-preserve, no-inherit;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"aaa"