(: Name: copynamespace-2 :)
(: Written by: Carmelo Montanez :)
(: Description: Evaluates that default namespace is overriden by local namespace.:)

declare copy-namespaces preserve,no-inherit;
declare default element namespace "http://example.org/names";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $new := <newElement xmlns = "http://www.example.com/mynamespace">{element original {"Original Content"}}</newElement>
return $new//*