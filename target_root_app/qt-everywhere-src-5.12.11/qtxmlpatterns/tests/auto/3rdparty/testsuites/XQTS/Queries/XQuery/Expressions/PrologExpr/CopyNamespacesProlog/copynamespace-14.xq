(: Name: copynamespace-14 :)
(: Written by: Carmelo Montanez :)
(: Description: Evaluates copy namespace declaration with value set to "preserve inherit" .:)
(: Use global variables where prefix differes in case.:)

declare copy-namespaces preserve,inherit;

declare variable $existingElement := <existingElement xmlns:somespace="http://www.existingnamespace.com">{"Existing Content"}</existingElement>;
declare variable $new := <newElement xmlns:SOMESPACE="http://www.another.com">{$existingElement}</newElement>;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in (in-scope-prefixes($new/existingElement))
order by exactly-one($var) ascending
return $var
