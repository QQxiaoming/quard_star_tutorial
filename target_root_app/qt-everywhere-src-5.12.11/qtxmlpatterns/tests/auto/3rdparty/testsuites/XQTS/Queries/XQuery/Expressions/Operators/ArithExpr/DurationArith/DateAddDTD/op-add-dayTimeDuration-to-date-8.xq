(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-date-8                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-date" operator that  :)
(:returns a negative value.                              :)
(:*******************************************************:)
 
(xs:date("0001-01-01Z") + xs:dayTimeDuration("-P11DT02H02M"))