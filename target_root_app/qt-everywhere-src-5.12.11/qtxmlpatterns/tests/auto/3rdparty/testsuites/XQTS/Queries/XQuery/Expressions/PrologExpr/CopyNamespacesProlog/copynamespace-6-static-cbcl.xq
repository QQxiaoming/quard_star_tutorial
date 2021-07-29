(: Name: copynamespace-6 :)
(: Written by: Carmelo Montanez :)
(: Description: Evaluates copy namespace declaration with value set to "preserve inherit" .:)
(: The copies element use the same prefix. :)

declare namespace foo = "http://example.org";
declare copy-namespaces preserve,inherit;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $existingElement := <foo:existingElement xmlns="http://www.existingnamespace.com">{"Existing Content"}</foo:existingElement>
let $new := <foo:newElement xmlns = "http://www.mynamespace.com">{$existingElement}</foo:newElement>
for $var in (in-scope-prefixes(exactly-one($new//child::*)))
order by exactly-one($var) ascending
return $var
