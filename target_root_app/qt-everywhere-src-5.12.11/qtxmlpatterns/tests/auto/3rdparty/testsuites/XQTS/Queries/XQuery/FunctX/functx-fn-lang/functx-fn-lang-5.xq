(:**************************************************************:)
(: Test: functx-fn-lang-5                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";

let $in-xml := <desclist xml:lang="en">
  <desc xml:lang="en-US">
     <line>A line of text.</line>
  </desc>
  <desc xml:lang="fr">
     <line>Une ligne de texte.</line>
  </desc>
</desclist>
return ($in-xml[lang('en-US')])
