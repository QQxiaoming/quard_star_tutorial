(:*******************************************************:)
(:Written By: Andrew Eisenberg                           :)
(:Date: March 28, 2006                                   :)
(:Purpose: Test of Trivial Embedding of XQuery           :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<xqx:xquery xmlns:xqx="http://www.w3.org/2005/XQueryX">for $i in (1 to 5) let $j := 4 where $i &lt; $j return &lt;x&gt;&lt;i&gt;&#x7b;$i&#x7d;&lt;/i&gt;&lt;/x&gt;</xqx:xquery>
