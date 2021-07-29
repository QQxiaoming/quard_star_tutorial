xquery version "1.1";

(: analyze-string with empty sequence :)

<out>
<a>{analyze-string((), "abc")}</a>
<b>{count(analyze-string((), "abc"))}</b>
</out>