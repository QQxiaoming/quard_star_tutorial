(: Name : validateexpr-29 :)
(: Description: Test For error condition XQDY0061 using a document node.:)
(: Author: Tim Mills :)
(: Date: 2010-03-15 :)

(: insert-start :)
import schema namespace ns="http://www.w3.org/XQueryTestOrderBy";
(: insert-end :)

validate { document { <ns:Strings/>, <ns:Strings/> } }
