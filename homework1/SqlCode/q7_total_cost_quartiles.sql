SELECT CompanyName, CustomerId, allcost
FROM ( 
		SELECT *, NTILE(4) OVER (ORDER BY allcost ASC) AS quartile
    FROM (
			  SELECT
        IFNULL(Customer.CompanyName, 'MISSING_NAME') AS CompanyName,
        'Order'.CustomerId,
        ROUND(SUM(OrderDetail.Quantity * OrderDetail.UnitPrice), 2) AS allcost
				FROM 'Order'
				INNER JOIN OrderDetail on OrderDetail.OrderId = 'Order'.Id
				LEFT JOIN Customer on Customer.Id = 'Order'.CustomerId
				GROUP BY 'Order'.CustomerId
		)
)
WHERE quartile = 1
ORDER BY allcost ASC
;