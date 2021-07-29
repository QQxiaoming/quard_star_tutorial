(: Name: Constr-namespace-27 :)
(: Written by: Andreas Behm :)
(: Description: Namespace Declarations - test namespace added to statically known namespaces: attribute content :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<elem xmlns:cm="http://www.example.com" attr="{count($input-context//cm:b)}"/>