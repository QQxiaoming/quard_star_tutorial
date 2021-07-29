(: Name: Axes077 :)
(: Description: Test implied child axis in '//*' after an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center//*)
