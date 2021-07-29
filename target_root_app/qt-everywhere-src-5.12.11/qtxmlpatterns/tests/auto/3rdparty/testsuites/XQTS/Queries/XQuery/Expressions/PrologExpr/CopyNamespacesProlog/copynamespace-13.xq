(: Name: copynamespace-13 :)
(: Written by: Carmelo Montanez :)
(: Description: Evaluates copy namespace declaration with value set to "preserve inherit" .:)
(: Use global variables. only the XML prefix is used.:)

declare copy-namespaces preserve,inherit;

declare variable $existingElement := <existingElement>{"Existing Content"}</existingElement>;
declare variable $new := <newElement>{$existingElement}</newElement>;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in (in-scope-prefixes(exactly-one($new/existingElement)))
return $var
