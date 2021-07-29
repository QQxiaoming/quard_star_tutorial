(: Name: fn-idref-30 :)
(: Description: Evaluation of fn:idref with various types derived from IDREF or IDREFS. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

<out xmlns="http://www.w3.org/XQueryTest/ididrefs"
     xmlns:i="http://www.w3.org/XQueryTest/ididrefs"
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">{
  for $z in tokenize("alpha beta gamma delta epsilon zeta eta theta iota kappa lambda mu", "\s")
  return
    element {$z} {idref($z, $input-context1)[self::attribute()], idref($z, $input-context1)[self::element()]}
}</out>

