(: Name: Constr-namespace-29 :)
(: Written by: Andreas Behm :)
(: Description: Namespace Declarations - test namespace removed from statically known namespaces after end of element :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<parent><child xmlns:cm="http://www.example.com"/><child>{count($input-context//cm:b)}</child></parent>