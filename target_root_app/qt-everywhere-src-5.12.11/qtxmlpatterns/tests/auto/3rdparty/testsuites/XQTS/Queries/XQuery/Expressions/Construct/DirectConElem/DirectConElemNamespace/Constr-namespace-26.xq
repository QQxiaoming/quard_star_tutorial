(: Name: Constr-namespace-26 :)
(: Written by: Andreas Behm :)
(: Description: Namespace Declarations - test namespace added to statically known namespaces: element content :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<elem xmlns:cm="http://www.example.com">{count($input-context//cm:b)}</elem>