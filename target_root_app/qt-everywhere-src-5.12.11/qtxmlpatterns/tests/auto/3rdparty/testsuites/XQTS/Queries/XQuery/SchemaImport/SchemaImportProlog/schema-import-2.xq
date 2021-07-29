(: Name: schema-import-2:)
(: Description: Evaluation of a prolog with two schema import that specifies the same namespace. :)

import schema namespace ns1 = "http://www.w3.org/XQueryTestOrderBy";
import schema namespace ns2 = "http://www.w3.org/XQueryTestOrderBy";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"abc"