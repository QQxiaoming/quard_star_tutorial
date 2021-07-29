(:*******************************************************:)
(: Test: fn-union-node-args-022.xq                       :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Tue May 24 03:34:54 2005                        :)
(: Purpose: arg: text node & text node                     :)
(:*******************************************************:)

(: insert-start :)
declare namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context external;
(: insert-end :)

($input-context/atomic:root/atomic:idrefs/text()) | ($input-context/atomic:root/atomic:idrefs/text())
