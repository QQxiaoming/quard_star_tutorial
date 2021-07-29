(: #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF] :)
declare variable $codepoints as xs:integer+ := (9,                  (: 0x9 :)
                                                10,                 (: 0xA :)
                                                13,                 (: 0xD :)
                                                32 to 55295,        (: 0x20 - 0xD7FF :)
                                                57344 to 65532,     (: 0xE000 - 0xFFFD :)
                                                65536 to 1114111    (: 0x10000 - 0x10FFFF :));
declare variable $count as xs:integer := count($codepoints);
declare variable $lineWidth as xs:integer := 70;

<allCodepoints>
    <!-- Each <r>-element represents a codepoint range. The 's' attribute
         is the start codepoint, the 'e' attribute is the end codepoint.
         Note that these are only *Hints*, since the character range is not contiguous.
      -->
{
    "&#xA;",
    "&#xA;",
    (: The outputted file is rather big, so to make it managable, we output
       a chunk of $lineWidth characters in each element.
     :)
    for $i in (1 to $count idiv $lineWidth)
    let $startOffset := (($i - 1) * $lineWidth) + 1
    return (<r s="{$codepoints[$startOffset]}"
               e="{exactly-one($codepoints[$startOffset]) + $lineWidth}">
                {
                        codepoints-to-string(subsequence($codepoints, $startOffset, $lineWidth))
                }
           </r>, "&#xA;")
}
</allCodepoints>

(: vim: et:ts=4:sw=4:sts=4
:)
