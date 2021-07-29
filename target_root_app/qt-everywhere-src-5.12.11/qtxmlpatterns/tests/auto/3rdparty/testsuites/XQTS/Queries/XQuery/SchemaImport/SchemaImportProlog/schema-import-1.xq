(: Name: schema-import-1:)
(: Description: Evaluation of a prolog with a schema import that should be hard to find :)

import schema namespace aSpace="http://www.youcannotfindthisschemaorg/schemas" at "http://www.youcannotfindithere/noschemas";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"abc"