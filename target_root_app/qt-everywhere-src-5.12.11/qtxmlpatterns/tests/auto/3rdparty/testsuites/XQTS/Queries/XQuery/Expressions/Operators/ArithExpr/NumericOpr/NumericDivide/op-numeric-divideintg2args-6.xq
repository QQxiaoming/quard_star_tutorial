(:*******************************************************:)
(:Test: op-numeric-divideintg2args-6                     :)
(:Written By: Sorin Nasoi                                :)
(:Date: 2009-04-01+02:00                                 :)
(:Purpose: Evaluates the type of the result of           :)
(:"op:numeric-integer-divide" operator with both         :)
(: arguments of type xs:integer                          :)
(:*******************************************************:)

(xs:integer("-999999999999999999") div xs:integer("999999999999999999")) instance of xs:decimal