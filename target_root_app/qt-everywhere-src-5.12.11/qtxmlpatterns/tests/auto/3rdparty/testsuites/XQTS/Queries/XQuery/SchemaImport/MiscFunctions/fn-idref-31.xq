(: Name: fn-idref-31 :)
(: Description: Evaluation of fn:idref with various types derived from IDREF or IDREFS. :)
(: tests Erratum E29 - use of a list-valued element/attribute as input to idref() :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

<out xmlns="http://www.w3.org/XQueryTest/ididrefs"
     xmlns:i="http://www.w3.org/XQueryTest/ididrefs"
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">{
  for $z in ($input-context1//i:IDREFS-content, $input-context1//i:IDREF-List-content, $input-context1//i:W/@i:Restricted-IDREF)
  for $s in distinct-values(tokenize(normalize-space(string($z)), '\s'))
  order by $s
  return
    <e val="{$s}">
      {idref($s, $input-context1)[self::attribute()], idref($s, $input-context1)[self::element()]}
    </e>
}</out>

