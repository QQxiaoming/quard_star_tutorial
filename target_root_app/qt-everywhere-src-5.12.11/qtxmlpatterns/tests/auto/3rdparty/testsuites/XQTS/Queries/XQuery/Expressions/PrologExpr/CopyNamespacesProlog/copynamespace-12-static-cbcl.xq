(: Name: copynamespace-12 :)
(: Written by: Carmelo Montanez :)
(: Description: Evaluates copy namespace declaration with value set to "preserve inherit" .:)
(: Use global variables and namespaces with prefixes. diferent prefix same namespace URI.:)

declare copy-namespaces preserve,inherit;

declare variable $existingElement := <existingElement xmlns:existingNamespace="http://www.existingnamespace.com">{"Existing Content"}</existingElement>;
declare variable $new := <newElement xmlns:newNamespace = "http://www.existingnamespace.com">{$existingElement}</newElement>;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in (in-scope-prefixes(exactly-one($new/existingElement)))
order by $var ascending return $var
