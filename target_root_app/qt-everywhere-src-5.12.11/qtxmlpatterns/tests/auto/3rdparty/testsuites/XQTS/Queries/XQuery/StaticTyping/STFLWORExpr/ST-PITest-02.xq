(: Name: ST-PI-01 :)
(: Description:
         In LetClause, binding expr's ST must be subtype of variable's ST.
         Under REC FS, both are 'processing-instruction?', so STA succeeds.
         Under PER FS, former is 'processing-instruction nonexistent',
            latter is 'processing-instruction filesytem', so STA fails, raises error.
         (Note that an implementation that doesn't do STA will not raise an error,
         because the LetClause isn't evaluated.)
:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $pi in $input-context//processing-instruction(nonexistent)
let $pi2 as processing-instruction(filesystem) := $pi
return $pi2
