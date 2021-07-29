(: Name: copynamespace-3 :)
(: Written by: Carmelo Montanez :)
(: Description: Evaluates copy namespace declaration with value set to "preserve no-inherit".:)

declare namespace foo = "http://example.org";
declare copy-namespaces preserve,no-inherit;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $existingElement := <existingElement xmlns="http://www.existingnamespace.com">{"Existing Content"}</existingElement>
let $new := <foo:newElement xmlns = "http://www.mynamespace.com">{$existingElement}</foo:newElement>
return in-scope-prefixes(exactly-one($new/child::node()))
