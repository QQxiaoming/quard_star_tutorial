(: Name: schema-import-3:)
(: Description: Evaluation of a prolog with two schema import that specifies no target namespace and specifies a prefix. :)

import schema namespace ns1 = "";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"abc"