<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="xml" omit-xml-declaration="yes" indent="yes"/>
	
	<xsl:template match="@* | node()">
		<xsl:copy>
			<xsl:apply-templates select="@* | node()"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template match="enumeration/element[@base-value]">
		<xsl:variable name="baseTypeRaw" select="../@base-data-type"/>
		<xsl:variable name="baseTypeName" select="translate($baseTypeRaw, ' ', '-')"/>
		<xsl:copy>
			<xsl:apply-templates select="@*[name() != 'base-value']"/>
			<xsl:apply-templates select="text()[1]"/>
			<base-value>
				<xsl:apply-templates select="text()[1]"/>
				<xsl:text disable-output-escaping="yes">&#x9;</xsl:text>
				<xsl:element name="{$baseTypeName}">
					<xsl:value-of select="@base-value"/>
				</xsl:element>
				<xsl:apply-templates select="text()[1]"/>
			</base-value>
			<xsl:apply-templates select="node()"/>
		</xsl:copy>
	</xsl:template>
</xsl:stylesheet>
