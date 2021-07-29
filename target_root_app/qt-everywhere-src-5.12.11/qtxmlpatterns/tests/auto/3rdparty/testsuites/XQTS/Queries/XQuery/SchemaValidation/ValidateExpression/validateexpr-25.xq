(: name : validateexpr-25 :)
(: Description: Test for Error condition XQDY0027.  Dynamic error during validation.:)

(: insert-start :)
import schema namespace simple="http://www.w3.org/XQueryTest/simple"; 
declare variable $input-context1 external;
(: insert-end :)

validate strict {<simple:duration>sorry, no duration here</simple:duration>}