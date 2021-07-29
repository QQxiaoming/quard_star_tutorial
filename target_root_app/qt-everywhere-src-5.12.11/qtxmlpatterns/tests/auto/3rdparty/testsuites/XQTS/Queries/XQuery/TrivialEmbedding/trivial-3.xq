(:*******************************************************:)
(:Written By: Andrew Eisenberg                           :)
(:Date: March 28, 2006                                   :)
(:Purpose: Test of Trivial Embedding of XQuery           :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<xqx:xquery xmlns:xqx="http://www.w3.org/2005/XQueryX"><![CDATA[for $i in (1 to 5) let $j := 4 where $i < $j return <x><i>{$i}</i></x>]]></xqx:xquery>
